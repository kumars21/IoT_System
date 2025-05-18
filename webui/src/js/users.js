$(function() {
    const iconTrash = `
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16">
            <path d="M2.5 1a1 1 0 0 0-1 1v1a1 1 0 0 0 1 1H3v9a2 2 0 0 0 2 2h6a2 2 0 0 0 2-2V4h.5a1 1 0 0 0 1-1V2a1 1 0 0 0-1-1H10a1 1 0 0 0-1-1H7a1 1 0 0 0-1 1zm3 4a.5.5 0 0 1 .5.5v7a.5.5 0 0 1-1 0v-7a.5.5 0 0 1 .5-.5M8 5a.5.5 0 0 1 .5.5v7a.5.5 0 0 1-1 0v-7A.5.5 0 0 1 8 5m3 .5v7a.5.5 0 0 1-1 0v-7a.5.5 0 0 1 1 0"/>
        </svg>
    `;

    const iconAdd = `
        <svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" fill="currentColor" viewBox="0 0 16 16">
          <path d="M1 14s-1 0-1-1 1-4 6-4 6 3 6 4-1 1-1 1zm5-6a3 3 0 1 0 0-6 3 3 0 0 0 0 6"/>
          <path fill-rule="evenodd" d="M13.5 5a.5.5 0 0 1 .5.5V7h1.5a.5.5 0 0 1 0 1H14v1.5a.5.5 0 0 1-1 0V8h-1.5a.5.5 0 0 1 0-1H13V5.5a.5.5 0 0 1 .5-.5"/>
        </svg>
    `;

    const $table = $("#users-table");
    const $addName = $("#users-new-name");
    const $addConfirm = $("#users-new-confirm");

    function setEnableState(enable) {
        $table.find("button").prop("disabled", !enable);
    }

    function insertItemRow(item) {
        const $row = $(`<tr>
            <td>${item.name}</td>
            <td>${item.tag}</td>
            <td><button class="btn btn-secondary btn-sm">${iconTrash}</button></td>
            </tr>`);
        $table.find("> tbody > tr").last().before($row);

        // handle delete callback
        $row.find("button").on("click", function() {
            setEnableState(false);
            $.ajax({
                url: process.env.APP_API_SERVER + "/api/users/delete",
                type: "POST",
                contentType: 'application/json',
                data: JSON.stringify({
                    "tag": item.tag,
                }),
            }).done(function(data) {
                if (data.succeed)
                    $row.remove();
            }).always(function() {
                setEnableState(true);
            });
        });
    }

    // install the icon
    $addConfirm.append(iconAdd);

    // pull user list
    $table.hide();
    $.ajax({
        url: process.env.APP_API_SERVER + "/api/users/get",
        type: "GET",
    }).done(function(data) {
        $table.show();
        $.each(data.users, function(idx, item) {
            insertItemRow(item);
        });
    });

    // clear tooltip if the field content has been changed
    $addName.on("input", function() {
        $addName.removeClass("is-invalid");
    });

    // handle add user request
    $addConfirm.on("click", function() {
        // reject empty user name
        const name = $.trim($addName.val());
        if (!name.length) {
            $addName.addClass('is-invalid');
            return;
        }

        setEnableState(false);
        $.ajax({
            url: process.env.APP_API_SERVER + "/api/users/add",
            type: "POST",
            contentType: 'application/json',
            data: JSON.stringify({
                "name": name,
            }),
        }).done(function(data) {
            if (data.succeed) {
                $addName.val("");
                insertItemRow(data.item);
            }
        }).always(function() {
            setEnableState(true);
        });
    });
});